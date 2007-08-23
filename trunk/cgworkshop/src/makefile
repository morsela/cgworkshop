include defs.mk


all: colorize

gmm-objs:=GMM.o

gui-objs:=GUI.o Loader.o Scribble.o ControlPanel.o main.o

graph-objs:=Segmentator.o GraphHandler.o

maxflow-objs:=graph.o maxflow.o

fe-objs:=FeatureExtraction.o cvgabor.o

app-objs:=$(gmm-objs) $(gui-objs) $(graph-objs) $(maxflow-objs) $(fe-objs)

gmm:
	make -C GMM objs='$(gmm-objs)'

gmm-clean:
	make -C GMM objs='$(gmm-objs)' clean

gui:
	make -C GUI objs='$(gui-objs)'

gui-clean:
	make -C GUI objs='$(gui-objs)' clean

FE:
	make -C fe objs='$(fe-objs)'

fe-clean:
	make -C fe objs='$(fe-objs)' clean

maxflow:
	make -C MaxFlow objs='$(maxflow-objs)'

maxflow-clean:
	make -C MaxFlow objs='$(maxflow-objs)' clean

graph: $(graph-objs)
	echo 'moo'

graph-clean:
	rm $(graph-objs)

clean: gmm-clean gui-clean graph-clean maxflow-clean fe-clean
	rm $(app-objs) colorize

colorize: gmm gui graph maxflow	FE app

app:
	$(CPP) $(app-objs) $(CPPFLAGS) $(LDFLAGS) -o colorize

include Rules.make

