{
  description = "Binding generator for Squirrel";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-23.05";

    tinycmmc.url = "github:grumbel/tinycmmc";
    tinycmmc.inputs.nixpkgs.follows = "nixpkgs";

    squirrel.url = "github:grumnix/squirrel";
    squirrel.inputs.nixpkgs.follows = "nixpkgs";
    squirrel.inputs.tinycmmc.follows = "tinycmmc";
  };

  outputs = { self, nixpkgs, tinycmmc, squirrel }:
    tinycmmc.lib.eachSystemWithPkgs (pkgs:
      {
        packages = rec {
          default = miniswig;

          miniswig = pkgs.callPackage ./miniswig.nix {
            squirrel = squirrel.packages.${pkgs.system}.default;
          };
        };
      }
    );
}
