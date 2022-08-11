mod proto {
  tonic::include_proto!("inner");
}

use crate::constants::MPSC_CHANNEL_BUFFER_CAPACITY;
use crate::tools::match_for_io_error;
use futures::Stream;
use std::{io::ErrorKind, pin::Pin};
use tokio::sync::mpsc;
use tokio_stream::{wrappers::ReceiverStream, StreamExt};
use tonic::{Request, Response, Status, Streaming};
use tracing::error;

pub use proto::inner_service_server::InnerServiceServer;

use proto::{
  inner_service_server::InnerService, TalkBetweenServicesRequest,
  TalkBetweenServicesResponse,
};

type TalkWithClientResult<T> = Result<Response<T>, Status>;
type ResponseStream = Pin<
  Box<dyn Stream<Item = Result<TalkBetweenServicesResponse, Status>> + Send>,
>;

#[derive(Debug)]
pub struct MyInnerService {}

#[tonic::async_trait]
impl InnerService for MyInnerService {
  type TalkBetweenServicesStream = ResponseStream;

  async fn talk_between_services(
    &self,
    req: Request<Streaming<TalkBetweenServicesRequest>>,
  ) -> TalkWithClientResult<ResponseStream> {
    println!("talk between services server start");

    let mut in_stream = req.into_inner();
    let (server_sender, server_receiver) =
      mpsc::channel(MPSC_CHANNEL_BUFFER_CAPACITY);

    tokio::spawn(async move {
      while let Some(request_result) = in_stream.next().await {
        match request_result {
          Ok(request) => {
            println!("received msg: {}", request.msg);
          }
          Err(err) => {
            if let Some(io_err) = match_for_io_error(&err) {
              if io_err.kind() == ErrorKind::BrokenPipe {
                error!("\tclient disconnected: broken pipe");
                break;
              }
            }

            match server_sender.send(Err(err)).await {
              Ok(_) => (),
              Err(_err) => {
                error!("response dropped");
                break;
              }
            }
          }
        }
      }
      println!("\tstream ended");
    });

    let out_stream = ReceiverStream::new(server_receiver);
    Ok(Response::new(Box::pin(out_stream) as ResponseStream))
  }
}
