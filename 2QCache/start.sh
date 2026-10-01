make
for file in tests/*; do
    ./2qcache.out "$file"
done
