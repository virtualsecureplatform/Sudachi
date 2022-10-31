scalaVersion := "2.13.8"

addCompilerPlugin("edu.berkeley.cs" % "chisel3-plugin" % "3.5.3" cross CrossVersion.full)

resolvers ++= Seq(
  Resolver.sonatypeRepo("snapshots"),
  Resolver.sonatypeRepo("releases")
)

libraryDependencies += "edu.berkeley.cs" %% "chisel3" % "3.5.3"
