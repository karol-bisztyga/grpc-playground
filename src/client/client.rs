mod proto {
  tonic::include_proto!("outer");
}

#[path = "../constants.rs"]
mod constants;

use tonic::Request;

use constants::{CLIENT_HOSTNAME, OUTER_SERVER_PORT};

pub use proto::outer_service_client::OuterServiceClient;
pub use proto::outer_service_server::OuterServiceServer;

use proto::TalkWithClientRequest;

pub async fn execute(
  index: usize,
  messages: &Vec<String>,
) -> Result<(), Box<dyn std::error::Error>> {
  let messages_cloned = messages.clone();
  let address = format!("{}:{}", CLIENT_HOSTNAME, OUTER_SERVER_PORT);
  println!("[{}] initializing the client {}", index, address);
  let mut client: OuterServiceClient<tonic::transport::Channel> =
    OuterServiceClient::connect(address).await?;

  let outbound = async_stream::stream! {
    println!("[{}] client sending {} messages", index, messages_cloned.len());
    for msg in messages_cloned {
      println!("[{}] - sending message: {}", index, msg);
      let request = TalkWithClientRequest {
        msg: msg.to_string(),
      };
      yield request;
    }
  };

  let response = client.talk_with_client(Request::new(outbound)).await?;
  let mut inbound = response.into_inner();
  while let Some(response) = inbound.message().await? {
    println!("[{}] got response: {}", index, response.msg);
  }

  Ok(())
}
