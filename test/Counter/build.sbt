scalaVersion := "2.12.12"
scalacOptions ++= Seq("-Xsource:2.11")

resolvers ++= Seq(
  Resolver.sonatypeRepo("snapshots"),
  Resolver.sonatypeRepo("releases")
)

val chiselGroupId = "edu.berkeley.cs"
libraryDependencies ++= Seq(
  chiselGroupId %% "chisel3" % "3.4.+",
  chiselGroupId %% "chisel-iotesters" % "1.5.+"
)