import chisel3._

class AdderMiniPort extends Bundle {
  val in_a = Input(UInt(16.W))
  val in_b = Input(UInt(16.W))
  val out = Output(UInt(17.W))
}

class AdderMini extends Module {
  val io = IO(new AdderMiniPort)

    io.out := io.in_a +& io.in_b 
}

object AdderMiniTop extends App {
  (new chisel3.stage.ChiselStage).emitVerilog(new AdderMini())
}