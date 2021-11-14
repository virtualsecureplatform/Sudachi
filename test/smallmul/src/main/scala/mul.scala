import chisel3._

class MulPort extends Bundle{
    val a = Input(UInt(4.W))
    val b = Input(UInt(4.W))
    val out = Output(UInt(8.W))
}

class Mul extends Module {
  val io = IO(new MulPort)

  io.out := io.a*io.b

}

object Elaborate extends App {
  chisel3.Driver.execute(args, () => new Mul())
}
