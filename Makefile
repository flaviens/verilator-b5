IMAGE_TAG ?= ethcomsec/verilator-b5

build_docker:
	docker build -t $(IMAGE_TAG) . 2>&1 | tee build.log

run_docker:
	docker run -it $(IMAGE_TAG)

push_docker:
	docker push $(IMAGE_TAG)
