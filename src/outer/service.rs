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
pub struct MyOuterService {
  // pub inner_client: Option<InnerClient>,
}

#[tonic::async_trait]
impl OuterService for MyOuterService {
  type TalkWithClientStream = ResponseStream;

  async fn talk_with_client(
    &self,
    req: Request<Streaming<TalkWithClientRequest>>,
  ) -> TalkWithClientResult<ResponseStream> {
    println!("talk with client start");

    let mut in_stream = req.into_inner();
    let (server_sender, server_receiver) =
      mpsc::channel(MPSC_CHANNEL_BUFFER_CAPACITY);

    // this spawn here is required if you want to handle connection error.
    // If we just map `in_stream` and write it back as `out_stream` the `out_stream`
    // will be drooped when connection error occurs and error will never be propagated
    // to mapped version of `in_stream`.
    tokio::spawn(async move {
      let inner_client = InnerClient {};
      let sender = inner_client.execute().await.unwrap();
      while let Some(request_result) = in_stream.next().await {
        match request_result {
          Ok(request) => {
            println!("received msg: {}", request.msg);
            println!("passing to the inner server");
            sender.send(request.msg).await.unwrap();
          }
          Err(err) => {
            if let Some(io_err) = match_for_io_error(&err) {
              if io_err.kind() == ErrorKind::BrokenPipe {
                // here you can handle special case when client
                // disconnected in unexpected way
                eprintln!("\tclient disconnected: broken pipe");
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
