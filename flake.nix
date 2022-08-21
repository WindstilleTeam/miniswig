{
  description = "Binding generator for Squirrel";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-22.05";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = nixpkgs.legacyPackages.${system};
      in {
        packages = rec {
          default = miniswig;

          miniswig = pkgs.stdenv.mkDerivation {
            pname = "miniswig";
            version = "0.0.0";

            src = nixpkgs.lib.cleanSource ./.;

            nativeBuildInputs = [
              pkgs.cmake
              pkgs.pkgconfig
            ];

            buildInputs = [
              pkgs.flex
              pkgs.bison
            ];
          };
        };
      }
    );
}
