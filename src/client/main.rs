mod proto {
  tonic::include_proto!("outer");
}

use rand::distributions::{Alphanumeric, DistString};

#[path = "../constants.rs"]
mod constants;

use tonic::Request;

use constants::{CLIENT_HOSTNAME, OUTER_SERVER_PORT};

pub use proto::outer_service_client::OuterServiceClient;
pub use proto::outer_service_server::OuterServiceServer;

use proto::TalkWithClientRequest;
use rand::Rng;

#[tokio::main]
async fn main() -> Result<(), Box<dyn std::error::Error>> {
  const N_MESSAGES: usize = 10;
  let address = format!("{}:{}", CLIENT_HOSTNAME, OUTER_SERVER_PORT);
  let mut rng = rand::thread_rng();
  println!("initializing the client {}", address);
  let mut client: OuterServiceClient<tonic::transport::Channel> =
    OuterServiceClient::connect(address).await?;
  
  println!("generating {} messages", N_MESSAGES);
  let mut messages = vec![];
  for _ in 0..N_MESSAGES {
    let size = rng.gen_range(40..70);
    messages.push(Alphanumeric.sample_string(&mut rng, size));
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
