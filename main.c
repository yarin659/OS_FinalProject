
#define UNREFERENCED_PARAMETER(x) (void)x;

int main(int argc, char *argv[]) {
    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);

    return 0;
}
