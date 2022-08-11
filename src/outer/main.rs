use tonic::transport::Server;

#[path = "../constants.rs"]
mod constants;
#[path = "../tools.rs"]
mod tools;

mod inner_client;
mod service;

use inner_client::InnerClient;
use service::{MyOuterService, OuterServiceServer};

use constants::{OUTER_SERVER_PORT, SERVER_HOSTNAME};

#[tokio::main]
async fn main() -> Result<(), Box<dyn std::error::Error>> {
  {
    println!("checking connection with the inner service...");
    let inner_client = InnerClient {};
    inner_client
      .check_connection()
      .await
      .expect("could not successfully connect to the inner server");
    println!("...done!");
  }

  let server = MyOuterService {};
  let address = format!("{}:{}", SERVER_HOSTNAME, OUTER_SERVER_PORT);
  Server::builder()
    .add_service(OuterServiceServer::new(server))
    .serve(address.parse()?)
    .await
    .unwrap();

  Ok(())
}
