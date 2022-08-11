mod proto {
  tonic::include_proto!("inner");
}

#[derive(Debug)]
pub struct InnerClient {}

use crate::constants::{
  CLIENT_HOSTNAME, INNER_SERVER_PORT, MPSC_CHANNEL_BUFFER_CAPACITY,
};

use tokio::sync::mpsc::{channel, Receiver, Sender};
use tonic::Request;

pub use proto::inner_service_client::InnerServiceClient;
pub use proto::TalkBetweenServicesRequest;

type InnerClientChannelType = String;

impl InnerClient {
  pub async fn check_connection(
    &self,
  ) -> Result<(), Box<dyn std::error::Error>> {
    let address = format!("{}:{}", CLIENT_HOSTNAME, INNER_SERVER_PORT);
    InnerServiceClient::connect(address).await?;

    Ok(())
  }

  pub async fn execute(
    &self,
  ) -> Result<Sender<InnerClientChannelType>, Box<dyn std::error::Error>> {
    let (sender, mut receiver): (
      Sender<InnerClientChannelType>,
      Receiver<InnerClientChannelType>,
    ) = channel(MPSC_CHANNEL_BUFFER_CAPACITY);

    tokio::spawn(async move {
      let address = format!("{}:{}", CLIENT_HOSTNAME, INNER_SERVER_PORT);
      println!("[inner client] initializing {}", address);
      let mut client: InnerServiceClient<tonic::transport::Channel> =
        InnerServiceClient::connect(address).await.unwrap();

      let outbound = async_stream::stream! {
        println!("[inner client] receiver start");
        while let Some(message) = receiver.recv().await {
          println!("[inner client] received message: {}", message);
            let request = TalkBetweenServicesRequest {
              msg: message.to_string(),
            };
            yield request;
        }
        println!("[inner client] receiver end");
      };

      let response = client
        .talk_between_services(Request::new(outbound))
        .await
        .unwrap();
      let mut inbound = response.into_inner();
      while let Some(response) = inbound.message().await.unwrap() {
        println!("[inner client] got response: {}", response.msg);
      }
    });

    Ok(sender)
  }
}
