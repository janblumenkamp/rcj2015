$fn = 100;
length = 17;
width = 27;

projection(cut = true)
translate([-30,0,15])
difference()
	{
		translate([27,0,-15.5])
		cube([36,33,3],true);
	
		hull() //Servobefestigung 1
		{
			translate([13,10,-18])
			cylinder(h=5,r=1.5);
			translate([13,-10,-18])
			cylinder(h=5,r=1.5);
		}
		
		translate([27,0,-16.5]) //Aussparung Servo
		minkowski()
		{
			cube([18,22,2],true);
			cylinder(h=2,r=2);
		}
		
		hull() //Servobefestigung 2
		{
			translate([41,10,-18])
			cylinder(h=5,r=1.5);
			translate([41,-10,-18])
			cylinder(h=5,r=1.5);
		}
		
		translate([7.5,13.5,-17.5]) //Aussparungen an den vier Ecken
		cube([7,4,4]);
	
		translate([7.5,-17.5,-17.5])
		cube([7,4,4]);
	
		translate([39.5,-17.5,-17.5])
		cube([7,4,4]);
	
		translate([39.5,13.5,-17.5])
		cube([7,4,4]);
	}