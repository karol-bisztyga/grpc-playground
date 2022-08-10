use tonic::transport::Server;

#[path = "../constants.rs"]
mod constants;
mod service;

use service::{MyOuterService, OuterServiceServer};

use constants::{OUTER_SERVER_PORT, SERVER_HOSTNAME};

#[tokio::main]
async fn main() -> Result<(), Box<dyn std::error::Error>> {
  let server = MyOuterService {};
  let address = format!("{}:{}", SERVER_HOSTNAME, OUTER_SERVER_PORT);
  Server::builder()
    .add_service(OuterServiceServer::new(server))
    .serve(address.parse()?)
    .await
    .unwrap();

  Ok(())
}
