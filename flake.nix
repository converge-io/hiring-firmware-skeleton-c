{
  description = "Hiring firmware skeleton C project with CMake and clang";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = nixpkgs.legacyPackages.${system};
      in
      {
        packages.default = pkgs.stdenv.mkDerivation {
          pname = "hiring-firmware-skeleton-c";
          version = "1.0.0";
          
          src = ./.;
          
          nativeBuildInputs = with pkgs; [
            cmake
            clang
          ];
          
          buildInputs = with pkgs; [
            # Add any runtime dependencies here
          ];
          
          cmakeFlags = [
            "-DCMAKE_C_COMPILER=clang"
            "-DCMAKE_BUILD_TYPE=Release"
          ];
          
          meta = with pkgs.lib; {
            description = "Hiring firmware skeleton C project";
            license = licenses.mit;
            platforms = platforms.all;
          };
        };

        devShells.default = pkgs.mkShell {
          buildInputs = with pkgs; [
            cmake
            clang
            gdb
            valgrind
          ];
          
          shellHook = ''
            echo "C development environment loaded"
            echo "Available tools: cmake, clang, gdb, valgrind"
          '';
        };
      });
}
