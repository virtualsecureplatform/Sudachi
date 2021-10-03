import chisel3._
// import Counter._

// Problem:
//
// Counter should be incremented by the 'amt'
//
class Counter extends Module {
  val io = IO(new Bundle {
    val amt = Input(UInt(4.W))
    val tot = Output(UInt(8.W))
  })

  val cnt = RegInit(0.U(8.W))
  cnt := cnt + io.amt

  io.tot := cnt
//  io.tot := Counter.counter(255.U, io.amt)

}

object CounterTop extends App {
  (new chisel3.stage.ChiselStage).emitVerilog(new Counter())
}