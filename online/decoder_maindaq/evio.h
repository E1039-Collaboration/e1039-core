/* prototypes */
#ifdef __cplusplus
extern "C" {
#endif

    int evOpen(char *filename, char *flags, int *handle);
    int evRead(int handle, int *buffer, int size);
    int evClose(int handle);


#ifdef __cplusplus
}

#endif
