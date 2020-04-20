ifeq ($(TARGET_BUILD_VARIANT),user)
obj-y += oppo_cfs_common_user.o
obj-y += oppo_cfs_mutex_user.o
obj-y += oppo_cfs_rwsem_user.o
obj-y += oppo_cfs_futex_user.o
else
obj-y += oppo_cfs_common.o
obj-y += oppo_cfs_mutex.o
obj-y += oppo_cfs_rwsem.o
obj-y += oppo_cfs_futex.o
endif
