touch ../test/fileop.log 2>&1 
echo "START" > ../test/fileop.log 2>&1
make ob >> ../test/fileop.log 2>&1 
echo "make cleanall" >> ../test/fileop.log 2>&1 
make kwest_libs >> ../test/fileop.log 2>&1 
echo "make kwest libraries" >> ../test/fileop.log 2>&1 
export LD_LIBRARY_PATH=$(LIB):$LD_LIBRARY_PATH
echo "export path" >> ../test/fileop.log 2>&1 
make >> ../test/fileop.log 2>&1 
echo "make project" >> ../test/fileop.log 2>&1 
echo "########################################" >> ../test/fileop.log 2>&1 
echo "mounting filesystem" >> ../test/fileop.log 2>&1 
./kwest mnt >> ../test/fileop.log 2>&1 
echo "mount complete" >> ../test/fileop.log 2>&1 
echo "#######################################" >> ../test/fileop.log 2>&1 
echo "testing filesystem operations" >> ../test/fileop.log 2>&1 
echo "#01 ls" >> ../test/fileop.log 2>&1 
ls -l mnt >> ../test/fileop.log 2>&1 
ls -l mnt/Audio >> ../test/fileop.log 2>&1 
ls -l mnt/Audio/Artist >> ../test/fileop.log 2>&1 
ls -l mnt/Audio/Album >> ../test/fileop.log 2>&1 
ls -l mnt/Audio/Genre >> ../test/fileop.log 2>&1 
ls -l mnt/Files >> ../test/fileop.log 2>&1 
ls -l mnt/Files/Music >> ../test/fileop.log 2>&1 
echo "" >> ../test/fileop.log 2>&1 
echo "#02 getattr" >> ../test/fileop.log 2>&1 
ls -l mnt/test.test >> ../test/fileop.log 2>&1 
fusermount -u mnt >> ../test/fileop.log 2>&1 
echo "END" >> ../test/fileop.log 2>&1 
