{ self
, stdenv
, cmake
, pkgconfig
, flex
, bison
}:

stdenv.mkDerivation {
  pname = "miniswig";
  version = "0.0.0";

  src = ./.;

  nativeBuildInputs = [
    cmake
    pkgconfig
    flex
    bison
  ];
}
