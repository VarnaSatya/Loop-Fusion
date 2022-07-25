void init(int *a, int *b, int *c, int n) {
  for (int i = 1; i < n; i++) {
    c[i-1] = c[i] + i;
    b[i] = i * i;
  } 

  for (int i = 0; i < n; i++) {
    a[i] = b[i] + c[i];
  } 
  
}
