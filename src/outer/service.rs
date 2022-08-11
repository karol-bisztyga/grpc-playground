mod proto {
  tonic::include_proto!("outer");
}

use crate::constants::MPSC_CHANNEL_BUFFER_CAPACITY;
use crate::inner_client::InnerClient;
use crate::tools::match_for_io_error;
use futures::Stream;
use std::{io::ErrorKind, pin::Pin};
use tokio::sync::mpsc;
use tokio_stream::{wrappers::ReceiverStream, StreamExt};
use tonic::{Request, Response, Status, Streaming};
use tracing::error;

pub use proto::outer_service_server::OuterServiceServer;

use proto::{
  outer_service_server::OuterService, TalkWithClientRequest,
  TalkWithClientResponse,
};

type TalkWithClientResult<T> = Result<Response<T>, Status>;
type ResponseStream =
  Pin<Box<dyn Stream<Item = Result<TalkWithClientResponse, Status>> + Send>>;

#[derive(Debug)]
pub struct MyOuterService {}

#[tonic::async_trait]
impl OuterService for MyOuterService {
  type TalkWithClientStream = ResponseStream;

  async fn talk_with_client(
    &self,
    req: Request<Streaming<TalkWithClientRequest>>,
  ) -> TalkWithClientResult<ResponseStream> {
    println!("[outer server] talk with client start");

    let mut in_stream = req.into_inner();
    let (server_sender, server_receiver) =
      mpsc::channel(MPSC_CHANNEL_BUFFER_CAPACITY);

    tokio::spawn(async move {
      let inner_client = InnerClient {};
      let client_sender = inner_client.execute().await.unwrap();
      while let Some(request_result) = in_stream.next().await {
        match request_result {
          Ok(request) => {
            println!("[outer server] received msg: {}", request.msg);
            println!("[outer server] passing to the inner server");
            client_sender.send(request.msg).await.unwrap();
          }
          Err(err) => {
            if let Some(io_err) = match_for_io_error(&err) {
              if io_err.kind() == ErrorKind::BrokenPipe {
                error!("[outer server] client disconnected: broken pipe");
                break;
              }
            }

            match server_sender.send(Err(err)).await {
              Ok(_) => (),
              Err(_err) => {
                error!("[outer server] response dropped");
                break;
              }
            }
          }
        }
      }
      println!("[outer server] stream ended");
    });

    let out_stream = ReceiverStream::new(server_receiver);
    Ok(Response::new(Box::pin(out_stream) as ResponseStream))
  }
}
