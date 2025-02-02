import subprocess

from utils import log, api_dir, use_shell


def build_api() -> None:
    """
    Builds the 'surf' api using GNU Make
    
    These make commands depend on the 'surf' codegen project being built
    and will build the surf project and install it as a side effect.
    """
    log("Building the surf api using make")

    server_cmd = subprocess.run(["make", "-C", api_dir, "server"], shell=use_shell)

    if server_cmd.returncode == 0:
        log(f"Successfully built server and interpreter shared library")
        log(f"Start the server with ./api/build/server/surf.exe")
    else:
        log("Failed to build the surf api")