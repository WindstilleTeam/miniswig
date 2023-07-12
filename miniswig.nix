{ self
, stdenv
, lib
, cmake
, pkgconfig
, flex
, bison
, squirrel
}:

stdenv.mkDerivation rec {
  pname = "miniswig";
  version = "0.0.0";

  src = ./.;

  # FIXME: miniswig.exe wants .dlls but can't find them
  doCheck = ! stdenv.targetPlatform.isWindows;

  cmakeFlags =
    lib.optional doCheck "-DBUILD_TESTS=ON";

  nativeBuildInputs = [
    cmake
    pkgconfig
    flex
    bison
  ];

  buildInputs =
    lib.optionals doCheck checkInputs;

  checkInputs = [
    squirrel
  ];
}
