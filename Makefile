build-c:
	@$(MAKE) -C raycasting-c

build-go:
	@$(MAKE) -C raycasting-go

run-c:
	@$(MAKE) -C raycasting-c run

run-go:
	@$(MAKE) -C raycasting-go run

clean-c:
	@$(MAKE) -C raycasting-c clean

clean-go:
	@$(MAKE) -C raycasting-go clean