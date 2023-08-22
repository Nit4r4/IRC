RESET = "\033[0m"
RED = "\033[31m"
GREEN =	"\033[32m"
YELLOW = "\033[33m"
BLUE = "\033[34m"
MAGENTA = "\033[35m"
CYAN = "\033[36m"
WHITE = "\033[37m"

def parse(data, port, origin):
	# Separar el string en una lista utilizando "\r\n" como delimitador
	full_msg = data.split(b"\r\n")
	# Recorrer la lista y procesar cada línea
	for line in full_msg:
		line_str = line.decode()
		if line_str.strip():
			if origin == "server":
				print (MAGENTA, "[", origin, ":", str(port), "]", line_str, RESET)
			else:
				print (GREEN, "[", origin, ":", str(port), "]", line_str, RESET)
