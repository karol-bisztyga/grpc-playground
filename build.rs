fn main() -> Result<(), Box<dyn std::error::Error>> {
  tonic_build::compile_protos("protos/inner.proto")?;
  tonic_build::compile_protos("protos/outer.proto")?;
  Ok(())
}
