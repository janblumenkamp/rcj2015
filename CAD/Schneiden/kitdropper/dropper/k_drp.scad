$fn = 100;
length = 17;
width = 27;
dropperpos = 0;

projection(cut = true)
color("white")
	{
		translate([dropperpos+5,0,0])
		difference()
		{
			translate([-15.5/2+1.5,0,0])
			cube([length+15+15.5,width-1,5],true);
			
			hull()
			{
				translate([length/2+2.5,-(width/2-3.5),-5])
				cylinder(h=10,r=2);
				translate([length/2+2.5,width/2-3.5,-5])
				cylinder(h=10,r=2);
			}
			translate([-21,0,0])
			cube([13,22,10],true);
		}
	}