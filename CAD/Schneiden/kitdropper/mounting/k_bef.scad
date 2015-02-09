$fn = 100;
length = 17;
width = 27;

projection(cut = true)
translate([0,-20,34])
difference() //Befestigungsleiste
		{
			translate([18,length/2+12,-34])
			cube([length+1+36,8,3],true);
			
			translate([0,length/2+13,-37])
			cylinder(h = 5, r = 1.5);
			translate([36,length/2+13,-37])
			cylinder(h = 5, r = 1.5);
		}