mod proto {
  tonic::include_proto!("outer");
}

#[path = "../constants.rs"]
mod constants;

pub use proto::outer_service_client::OuterServiceClient;
pub use proto::outer_service_server::OuterServiceServer;
use tokio::runtime::Runtime;

use rand::distributions::{Alphanumeric, DistString};
use rand::Rng;

mod client;

#[tokio::main]
async fn main() -> Result<(), Box<dyn std::error::Error>> {
  const N_CLIENTS: usize = 20;
  println!("starting {} clients", N_CLIENTS);

  let mut data = vec![];
  let mut rng = rand::thread_rng();

  for i in 0..N_CLIENTS {
    let n_messages = rng.gen_range(5..12);
    println!("generating {} messages for client {}", n_messages, i);
    let mut messages = vec![];
    for _ in 0..n_messages {
      let size = rng.gen_range(40..70);
      messages.push(Alphanumeric.sample_string(&mut rng, size));
    }
    data.push(messages);
  }

  let rt = Runtime::new().unwrap();
  tokio::task::spawn_blocking(move || {
    rt.block_on(async {
      let mut handlers = vec![];
      for (i, item) in data.iter().enumerate() {
        println!("running client number {}", i);
        let item_cloned = item.clone();
        handlers.push(tokio::spawn(async move {
          client::execute(i, &item_cloned).await.unwrap();
        }));
      }

      for handler in handlers {
        handler.await.unwrap();
      }
    });
  })
  .await
  .expect("Task panicked");

  Ok(())
}
