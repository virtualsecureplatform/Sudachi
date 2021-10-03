;buildInfoPackage: chisel3, version: 3.4.3, scalaVersion: 2.12.12, sbtVersion: 1.3.10
circuit Counter : 
  module Counter : 
    input clock : Clock
    input reset : UInt<1>
    output io : {flip amt : UInt<4>, tot : UInt<8>}
    
    reg cnt : UInt<8>, clock with : (reset => (reset, UInt<8>("h00"))) @[Counter.scala 14:20]
    node _T = add(cnt, io.amt) @[Counter.scala 15:14]
    node _T_1 = tail(_T, 1) @[Counter.scala 15:14]
    cnt <= _T_1 @[Counter.scala 15:7]
    io.tot <= cnt @[Counter.scala 17:10]
    
