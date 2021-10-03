;buildInfoPackage: chisel3, version: 3.4.0, scalaVersion: 2.12.12, sbtVersion: 1.3.10
circuit AdderMini : 
  module AdderMini : 
    input clock : Clock
    input reset : UInt<1>
    output io : {flip in_a : UInt<16>, flip in_b : UInt<16>, out : UInt<17>}
    
    node _io_out_T = add(io.in_a, io.in_b) @[AdderMini.scala 12:23]
    io.out <= _io_out_T @[AdderMini.scala 12:12]
    
