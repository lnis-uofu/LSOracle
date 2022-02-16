read_blif -a /mnt/nas/users/snelgrov/lsoracle/benchmarks/picorv32/picorv32.blif
external_partition -f /mnt/nas/users/snelgrov/lsoracle/benchmarks/picorv32/picorv32.parts
optimization_redux --abc_exec yosys-abc
