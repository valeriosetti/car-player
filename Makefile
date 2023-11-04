OUTPUT:=${shell pwd}/output
BR2_EXTERNAL:=${shell pwd}

.DEFAULT:
	$(MAKE) -C ./buildroot BR2_EXTERNAL=$(BR2_EXTERNAL) O=$(OUTPUT) "$@"
