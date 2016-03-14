all: build dot msc

BUILD_DIR=build
$(BUILD_DIR):
	mkdir $(BUILD_DIR)
clean:
	rm -r $(BUILD_DIR) || true

DOT_SRC=$(wildcard *.gv *.dot)
DOT_PNG=$(patsubst %.gv, $(BUILD_DIR)/%-dot.png, $(DOT_SRC))
$(BUILD_DIR)/%-dot.png: %.gv
	dot -T png -o $@ $^
dot: $(DOT_PNG)

MSC_SRC=$(wildcard *.msc)
MSC_PNG=$(patsubst %.msc, $(BUILD_DIR)/%-msc.png, $(MSC_SRC))
$(BUILD_DIR)/%-msc.png: %.msc
	mscgen -T png -o $@ $^
msc: $(MSC_PNG)

