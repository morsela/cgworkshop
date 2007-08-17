all: $(objs)
	cp $(objs) $(top)

clean:
	-rm $(objs)
