savedcmd_smartlamp.mod := printf '%s\n'   smartlamp.o | awk '!x[$$0]++ { print("./"$$0) }' > smartlamp.mod
