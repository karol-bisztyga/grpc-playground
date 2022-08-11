use tonic::transport::Server;

#[path = "../constants.rs"]
mod constants;
#[path = "../tools.rs"]
mod tools;

mod service;

use service::{InnerServiceServer, MyInnerService};

use constants::{INNER_SERVER_PORT, SERVER_HOSTNAME};

#[tokio::main]
async fn main() -> Result<(), Box<dyn std::error::Error>> {
  let server = MyInnerService {};
  let address = format!("{}:{}", SERVER_HOSTNAME, INNER_SERVER_PORT);
  Server::builder()
    .add_service(InnerServiceServer::new(server))
    .serve(address.parse()?)
    .await
    .unwrap();

  Ok(())
}
