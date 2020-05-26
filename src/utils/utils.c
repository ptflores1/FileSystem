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