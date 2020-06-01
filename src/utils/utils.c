int count_bits(unsigned char byte)
{
    int res = 0;
    for (int i = 0; i < 8; i++)
    {
        res += byte & 1;
        byte >>= 1;
    }
    return res;
}

int cmp_filename(unsigned char *dir_entry, char *filename)
{
    for (int i = 3; i < 32; i++)
    {
        if (dir_entry[i] != filename[i - 3])
            return 0;
        if (dir_entry[i] ==  '\0' && filename[i - 3] == '\0')
            return 1;
    }
    return 1;
}

void swap(unsigned char *a, unsigned char *b)
{
    unsigned char tmp = *a;
    *a = *b;
    *b = tmp;
}

void check_filename_length(char *filename, int length) {
    if (strlen(filename) > length) {
        printf("[ERROR] Filename \"%s\" is too long: %d.\n", filename, strlen(filename));
        exit(1);
    }
}