# GalaxyClassification
This is used to illustrate the process of this classification processing.

In this experiment, I did it in the following steps. 

1.I downloaded the images from Zoo Galaxy by the coordinatoors of galaxies. The data source is in file 'GalaxyZoo1_DR_table7.csv' and the code is in file 'downloadGalaxy.py'(Please create an account in http://www.sciserver.org/ and replace variables 'Authentication_loginName' and 'Authentication_loginPassword' with your own username and password).

2.I used the code in repo https://github.com/CarolineHaigh/mtobjects to detect the galaxies size(or say the pixel positions) in each of the images. 

So in this step you don't need to run the lines of code in mto.py 

```
mto.generate_image(image, id_map, params)
mto.generate_parameters(image, id_map, sig_ancs, params)
```

By using the id_map generated in code 
```
id_map = mto.relabel_segments(id_map, shuffle_labels=False) 
```
you could find out the id that contains the galaxy.

The follow is what I did with the id_map:
I know it's not a good solution (even a bit of verbosing). So please optimize it if you would like to do(which is also should be done). 
```
object_ids = id_map.ravel()
sorted_ids = object_ids.argsort()
id_set = list(set(object_ids))
# # print ("the length is %d" % len(id_set))
right_indices = np.searchsorted(object_ids, id_set, side='right', sorter=sorted_ids)
left_indices = np.searchsorted(object_ids, id_set, side='left', sorter=sorted_ids)

assumeXstart = 0
assumeXend = 0
assumeYstart = 0
assumeYend = 0
assumeArea = 0
assumeYs = []
assumeXs = []

for n in range(len(id_set)):
	pixel_indices = np.unravel_index(sorted_ids[left_indices[n]:right_indices[n]], processed_image.shape)

	width = np.amax(pixel_indices[1]) - np.amin(pixel_indices[1])
	height = np.amax(pixel_indices[0]) - np.amin(pixel_indices[0])
	
	if (width >= 180) and (height >= 180) and (width != 1023) and (height != 1023):
		currentCenterX = (assumeXstart + assumeXend) / 2.0
		currentCenterY = (assumeYstart + assumeYend) / 2.0

		newCenterX = (np.amax(pixel_indices[1]) + np.amin(pixel_indices[1])) / 2.0
		newCenterY = (np.amax(pixel_indices[0]) + np.amin(pixel_indices[0])) / 2.0
		print ("the widthh is %d and height is %d" % (width, height))

		if (pow(abs (currentCenterX - 512) , 2) + pow(abs(currentCenterY - 512) , 2)) > (pow(abs (newCenterX - 512), 2) + pow(abs(newCenterY - 512) ,2)) :
			assumeXstart = np.amin(pixel_indices[1])
			assumeXend = np.amax(pixel_indices[1])
			assumeYstart = np.amin(pixel_indices[0])
			assumeYend = np.amax(pixel_indices[0])
			assumeArea = len(pixel_indices[0])
			assumeYs = pixel_indices[0]
			assumeXs = pixel_indices[1]
			print ("The n is %d" % n)
```

3.Then I just cut only the areas contain the galaxies from each images.(I used the variables from last step - including assumeXstart, assumeXend, assumeYstart, assumeYend, assumeYs, assumeXs). 

4.I built max-trees based on the new generated images(contains only the galaxies). In the meantime, I calculated all the moments in each of node in max-trees and generated the local spectrums.

Again, I used the code from Caroline Haigh. But I replaced the original maxtree.h and maxtree.c to my new C files 'MTMaxTree.h' and 'MTMaxTree.c'. In these two files, I calculated the moments of each node during the constructing of max-tree and save the local spetrums to local files.

5.By the local spectrums generated from step 4, I used the GMLVQ(http://matlabserver.cs.rug.nl/gmlvqweb/web/) to train the model and observe the results.

For now I just classified two classes - elliptic and spiral. I think it would be better if SBa, SBb,SBc.. or Sa, Sb, Sc.. could be classified by using this approach.

