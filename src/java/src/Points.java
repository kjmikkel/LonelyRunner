/*
    This file is part of The Java Lonely Runner Checker.

    The Lonely Runner Checker is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    The Java Lonely Runner Checker is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with The Java Lonely Runner Checker.  If not, see <http://www.gnu.org/licenses/>.
*/

public class Points {

	int local_placement;
	int rounds;
	int num_runners;
	int speed;
	
	int runnerNum;
	
	public Points(int local, int rounds, int speed, int runner, int num_runners) {
		this.local_placement = local;
		this.rounds = rounds;
		this.speed = speed;
		this.runnerNum = runner;
		this.num_runners = num_runners;
	}
	
}