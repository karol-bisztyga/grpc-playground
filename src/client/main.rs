mod proto {
  tonic::include_proto!("outer");
}

use rand::distributions::{Alphanumeric, DistString};

#[path = "../constants.rs"]
mod constants;

use tonic::Request;

use constants::OUTER_SERVER_ADDR;

pub use proto::outer_service_client::OuterServiceClient;
pub use proto::outer_service_server::OuterServiceServer;

use proto::TalkWithClientRequest;

#[tokio::main]
async fn main() -> Result<(), Box<dyn std::error::Error>> {
  const N_MESSAGES: usize = 10;
  let mut rng = rand::thread_rng();
  let mut client: OuterServiceClient<tonic::transport::Channel> =
    OuterServiceClient::connect(OUTER_SERVER_ADDR).await?;
  
  let mut messages = vec![];
  for _ in 0..N_MESSAGES {
    messages.push(Alphanumeric.sample_string(&mut rng, 64));
  }

  let outbound = async_stream::stream! {
    println!("client sending {} messages", N_MESSAGES);
    for msg in messages {
      println!("- sending message: {}", msg);
      let request = TalkWithClientRequest {
        msg: msg,
      };
      yield request;
    }
  };

  let response = client.talk_with_client(Request::new(outbound)).await?;
  let mut inbound = response.into_inner();
  while let Some(response) = inbound.message().await? {
    println!("got response: {}", response.msg);
  }

  Ok(())
}
