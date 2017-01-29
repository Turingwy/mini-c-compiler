void int_to_str(int i, char *s) {
    int cur = 0;
    if(i == 0) {
        s[0] = '0';
        s[1] = '\0';
        return ;
    }
        
    while(i != 0) {
        s[cur++] = i%10 + '0'; 
        i/=10;
    }
    for(int k = 0; k < cur/2; k++) {
        char tmp = s[k];
        s[k] = s[cur-k-1];
        s[cur-k-1] = tmp;
    }
    s[cur] = '\0';
}

