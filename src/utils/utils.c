int count_bits(unsigned char byte){
    int res = 0;
    for (int i = 0; i < 8; i++)
    {
        res += byte & 1;
        byte >>= 1;
    }
    return res;
}