import chisel3._
import circt.stage.ChiselStage

class MulPort extends Bundle{
    val a = Input(UInt(16.W))
    val b = Input(UInt(16.W))
    val out = Output(UInt(32.W))
}

class Mul extends Module {
  val io = IO(new MulPort)

  io.out := io.a*io.b

}

object Elaborate extends App {
  ChiselStage.emitSystemVerilogFile(new Mul(),args = Array("-td", "."),firtoolOpts = Array("-disable-all-randomization", "-strip-debug-info"))
}
