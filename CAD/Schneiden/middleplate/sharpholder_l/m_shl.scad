$fn = 100;

module ir_mounting(height)
{
	difference()
	{
		translate([0,0,-height/2])
		cube([3,46,height],true);
	
		rotate([0,90,0])
		{
			translate([height-4.5,0,0])
			{
				translate([-1,(37/2),-2])
				cylinder(h=5,r=drill_m3/2);
				translate([-1,-(37/2),-2])
				cylinder(h=5,r=drill_m3/2);
			}
		}
	}
}

projection(cut = true)
rotate([0,90,0])
ir_mounting(30);