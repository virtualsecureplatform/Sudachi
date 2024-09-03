scalaVersion := "2.13.13"
val chiselVersion = "6.3.0"
addCompilerPlugin("org.chipsalliance" % "chisel-plugin" % chiselVersion cross CrossVersion.full)
// addCompilerPlugin("edu.berkeley.cs" % "chisel3-plugin" % "3.6.0" cross CrossVersion.full)

resolvers ++= Resolver.sonatypeOssRepos("releases")

libraryDependencies ++= Seq(
    "org.chipsalliance" %% "chisel" % chiselVersion,
    // "edu.berkeley.cs" %% "chisel3" % "3.6.0",
    "edu.berkeley.cs" %% "chiseltest" % "6.0.0"
)

scalacOptions ++= Seq(
      "-Xsource:2.13",
      "-language:reflectiveCalls",
      "-deprecation",
      "-feature",
      "-Xcheckinit"
      // Enables autoclonetype2 in 3.4.x (on by default in 3.5)
    //   "-P:chiselplugin:useBundlePlugin"
    )