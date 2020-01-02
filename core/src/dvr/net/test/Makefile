# which page numbers to fetch
numbers := $(shell seq 1 100)

# default target which depends on files 1.html .. 10.html
# (patsubst replaces % with %.html for each number)
all: $(patsubst %,%.cgi,$(numbers))

# the rule which tells how to generate a %.html dependency
# $@ is the target filename e.g. 1.html
1.cgi:
	curl "http://${IP}/cgi-bin/Config.cgi?action=dir&property=DCIM&count=20&format=all&from=1"
%.cgi:
	cs 'Camera.Menu.UIMode' 2>/dev/null
	
#cs 'Camera.Preview.H264.w' 2>/dev/null
