Usage:

    attach [-user] /dev/ttyX program [args]

Run the program with stdin/stderr/stderr attached to the specified tty.

Must run as root but if '-user' given then drop privileges to specified 'user'
before starting program.
