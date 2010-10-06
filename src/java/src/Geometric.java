import java.sql.Date;
import java.util.Calendar;
import java.util.Comparator;
import java.util.LinkedList;
import java.util.PriorityQueue;
import java.util.Random;

class finishingLinePoint extends Points {

	public finishingLinePoint(int local, int rounds, int speed, int runner, int num_runners) {
		super(local, rounds, speed, runner, num_runners);
	} 
	
}


class startTimePoint extends Points {

	public startTimePoint(int rounds, int speed, int runner, int num_runners) {
		super(1, rounds, speed, runner, num_runners);
	}
	
}

class endTimePoint extends Points {

	public endTimePoint(int rounds, int speed, int runner, int num_runners) {
		super(num_runners, rounds, speed, runner, num_runners);
	}
		
}

class returnValue {
	
	public double time;
	public boolean result;
	
}

class pointComparatorNoFloat implements Comparator<Points> {

	@Override
	public int compare(Points p, Points q) {

		if (p instanceof finishingLinePoint || q instanceof finishingLinePoint) {
			boolean finish = true;
		}
		
		int time1 = (p.local_placement + p.rounds * (p.num_runners + 1)) * q.speed;
		int time2 = (q.local_placement + q.rounds * (p.num_runners + 1)) * p.speed;
		
		if(time1 < time2) {
			return -1;
		} else if (time1 > time2) {
			return 1;
		} else if(p instanceof finishingLinePoint) {
			return -1;
		} else if (p instanceof startTimePoint && q instanceof endTimePoint) {
			return -1;
		} else if (p.runnerNum < q.runnerNum) {
			return -1;
		}
		
		return 1;
	}
	
}

public class Geometric {

	public static void main(String[] args) {
		LinkedList<Integer> list = new LinkedList<Integer>();
		
		Random ran = new Random();
		for(int index  = 0; index < 4; index++) {
		ran.setSeed(System.currentTimeMillis());
		
		for(int i = 0; i < 200; i++) {
			list.add(Math.abs(i));
		}
		
		Calendar now = Calendar.getInstance();

		

		double startTime = System.currentTimeMillis();
		returnValue returnValGeo = geometric(list);
		double endTime = System.currentTimeMillis();
		
		boolean resultTest = true;
	
		double rightside = 1.0 / (list.size() + 1);
		for(int speed : list) {
			double closeToInteger = closeToInteger(returnValGeo.time, speed);
			resultTest = resultTest && (closeToInteger >= rightside);
		}
				
		System.out.println(returnValGeo.result + ", " + resultTest + ": " + returnValGeo.time + ". Time taken: " + (endTime - startTime));
		
		
		startTime = System.currentTimeMillis();
		returnValue returnValNum = numeric(list);
		endTime = System.currentTimeMillis();
		
		System.out.println(returnValNum.result + ": " + returnValNum.time + ". Time taken: " + (endTime - startTime));
		}
		
	}
	
	public static PriorityQueue<Points> MakeTimePoints(endTimePoint p, PriorityQueue<Points> queue) {
		
		startTimePoint sPoint = new startTimePoint(p.rounds + 1, p.speed, p.runnerNum, p.num_runners);
		endTimePoint ePoint = new endTimePoint(p.rounds + 1, p.speed, p.runnerNum, p.num_runners);
				
		queue.add(sPoint);
		queue.add(ePoint);
		
		return queue;
	}
	
	private static double closeToInteger(double x, int w) {
		double input = (x * w) % 1.0;
		
		return Math.min(1 - input, input);
	}
	
	public static returnValue numeric(LinkedList<Integer> speeds) {
		
		int n = speeds.size();
		Object[] arraySpeedsTemp = speeds.toArray(); 
		
		Integer[] arraySpeeds = new Integer[n];
		for(int i = 0; i < n -1; i++) {
			arraySpeeds[i] = (Integer)arraySpeedsTemp[i];
		}
		
		for(int i = 0; i < n - 2; i++) {
			
			int speed_1 = arraySpeeds[i];
			
			for(int j = i + 1; j < n - 1; j++) {
				
				int speed_2 = arraySpeeds[j];		
				int k = speed_1 + speed_2;
				
				for(int a = 1; a < k - 1; a++) {
					boolean testValid = true;
				
					double doubleA = (double)a;
					
					for(Integer speed : speeds) {
						testValid = testValid && (closeToInteger(doubleA / k, speed) >= (1.0 / (n + 1) ));
						if (!testValid) {
							break;
						}
					}
					
					if (testValid) {
						returnValue returnVal = new returnValue();
						returnVal.result = true;
						returnVal.time = doubleA / k;
						return returnVal;
					}
					
				}
			}
		}
		
		returnValue returnval = new returnValue();
		returnval.result = false;
		
		return returnval;
	}
	
	public static returnValue geometric(LinkedList<Integer> speeds) {
		int intersect = 0;
		int numRunners = speeds.size();
		
		PriorityQueue<Points> pointQueue = new PriorityQueue<Points>(2 * numRunners + 1, new pointComparatorNoFloat());
		int runnerNum = 1;
		
		finishingLinePoint finPoint = new finishingLinePoint(numRunners + 1, 1, 1, -1, numRunners);
		pointQueue.add(finPoint);
		
		for(int speed : speeds) {
			
			startTimePoint sPoint = new startTimePoint(0, speed, runnerNum, numRunners);
			endTimePoint ePoint = new endTimePoint(0, speed, runnerNum, numRunners);
			pointQueue.add(sPoint);
			pointQueue.add(ePoint);
			runnerNum++;
		}
		
		int interSectMax = 0;
		int rounds = 0;
		
		while(pointQueue.size() > 0) {
			Points p = pointQueue.remove();
	//		System.out.println("size: " + pointQueue.size());
			
			
			if (p instanceof startTimePoint) {
				intersect++;
		
				if (intersect > interSectMax) {
					interSectMax++;					
				}
								
				if (intersect == numRunners) {
					returnValue returnVal = new returnValue();
					Double top = new Double(p.local_placement + p.rounds * (numRunners + 1));
					Double down = new Double((p.speed * (numRunners + 1)));
					returnVal.time = top / down;
					returnVal.result = true;
					
					return returnVal;
				}
			} else if (p instanceof endTimePoint) {
				intersect--;
				
				endTimePoint endTime = (endTimePoint)p;
						
				MakeTimePoints(endTime, pointQueue);	
				
				
			} else if (p instanceof finishingLinePoint) {
				returnValue returnval = new returnValue();
				returnval.result = false;
				
				return returnval;
			}
			rounds++;
		}
		
		returnValue returnval = new returnValue();
		returnval.result = false;
		
		return returnval;
	}
	
	
}


