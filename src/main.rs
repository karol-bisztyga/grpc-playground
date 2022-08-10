use tonic::transport::Server;

mod constants;
mod service;

use service::{MyOuterService, OuterServiceServer};

use constants::OUTER_SERVER_ADDR;

#[tokio::main]
async fn main() -> Result<(), Box<dyn std::error::Error>> {
  let server = MyOuterService {};
  Server::builder()
    .add_service(OuterServiceServer::new(server))
    .serve(OUTER_SERVER_ADDR.parse()?)
    .await
    .unwrap();

  Ok(())
}
