import chisel3._
import circt.stage.ChiselStage

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
  ChiselStage.emitSystemVerilogFile(new Counter(),args = Array("-td", "."),firtoolOpts = Array("-disable-all-randomization", "-strip-debug-info"))
}