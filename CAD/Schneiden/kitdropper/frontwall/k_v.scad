$fn = 100;
length = 17;
width = 27;

projection(cut = true)
rotate([0,90,0])
translate([8,0,0])
difference() //Vorderwand
		{
			translate([-(length/2-1),0,8.75]) 
			cube([3,width+6,74.5],true);
			
			translate([-9.5,13.5,-29])
			cube([4,5,20]);
			translate([-9.5,-18.5,-29])
			cube([4,5,20]);
	
			translate([-9.5,13.5,21])
			cube([4,5,30]);
			translate([-9.5,-18.5,21])
			cube([4,5,30]);
		}