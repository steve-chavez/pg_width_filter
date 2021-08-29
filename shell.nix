with import (builtins.fetchTarball {
  name = "2020-12-22";
  url = "https://github.com/NixOS/nixpkgs/archive/2a058487cb7a50e7650f1657ee0151a19c59ec3b.tar.gz";
  sha256 = "1h8c0mk6jlxdmjqch6ckj30pax3hqh6kwjlvp2021x3z4pdzrn9p";
}) {};
let
  width_filter = { postgresql }:
    stdenv.mkDerivation {
      name = "width_filter";
      buildInputs = [ postgresql ];
      src = ./.;
      installPhase = ''
        mkdir -p $out/bin
        install -D width_filter.so -t $out/lib
      '';
    };
  pgWithExt = { postgresql } :
    let pg = postgresql.withPackages (p: [ (width_filter {inherit postgresql;}) ]);
    in ''
      export PATH=${pg}/bin:"$PATH"

      tmpdir="$(mktemp -d)"

      export PGDATA="$tmpdir"
      export PGHOST="$tmpdir"
      export PGUSER=postgres
      export PGDATABASE=postgres

      trap 'pg_ctl stop -m i && rm -rf "$tmpdir"' sigint sigterm exit

      PGTZ=UTC initdb --no-locale --encoding=UTF8 --nosync -U "$PGUSER"

      options="-F -c listen_addresses=\"\" -k $PGDATA"

      ext_options="-c shared_preload_libraries=\"width_filter\""

      pg_ctl start -o "$options" -o "$ext_options"

      "$@"
    '';
  width-filter-pg-12 = writeShellScriptBin "width-filter-pg-12" (pgWithExt { postgresql = postgresql_12; });
  width-filter-pg-13 = writeShellScriptBin "width-filter-pg-13" (pgWithExt { postgresql = postgresql_13; });
in
mkShell {
  buildInputs = [ width-filter-pg-12 width-filter-pg-13 ];
}
