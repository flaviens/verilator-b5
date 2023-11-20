# Simulation inaccurate when running without traces

## Setting up

We provide a Docker image for better reproducibility.

You can either pull the image or make it.

To pull the Docker image:

```
docker pull docker.io/ethcomsec/verilator-b5
```

To make the Docker image:

```
make build_docker
```

Then, to run the Docker image:

```
make run_docker
```

## Observing the mismatch

To observe the mismatch, run the following commands:

Inside the Docker container, run:
```
diff *.log
```
This will show the diff between the runs with and without traces.
