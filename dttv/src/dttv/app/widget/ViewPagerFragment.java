package dttv.app.widget;

import android.annotation.SuppressLint;
import android.os.Bundle;
import android.support.v4.app.Fragment;
import android.support.v4.app.FragmentManager;
import android.support.v4.app.FragmentPagerAdapter;
import android.support.v4.view.ViewPager;
import android.support.v4.view.ViewPager.OnPageChangeListener;
import android.util.DisplayMetrics;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.ViewGroup.LayoutParams;
import android.view.animation.LinearInterpolator;
import android.view.animation.TranslateAnimation;
import android.widget.ImageView;
import android.widget.RadioButton;
import android.widget.RadioGroup;
import android.widget.RadioGroup.OnCheckedChangeListener;
import android.widget.Toast;

import com.actionbarsherlock.app.SherlockFragment;

import dttv.app.R;
import dttv.app.utils.Constant;

@SuppressLint("ValidFragment")
public class ViewPagerFragment extends SherlockFragment {
	private View mRootView;
	private ViewPager mViewPager;
	private int indicatorWidth = 0;
	private RadioGroup tabGroup;
	private ImageView iv_nav_indicator;
	
	LayoutInflater mInflater;
	static String[] tabs;
	
	TabFragmentPagerAdapter mAdapter;
	private int currentIndicatorLeft = 0;
	ChangeActionModeListener modeListener;
	
	public ViewPagerFragment(ChangeActionModeListener changeActionModeListener){
		modeListener = changeActionModeListener;
	}
	
	@Override
	public void onCreate(Bundle savedInstanceState) {
		// TODO Auto-generated method stub
		super.onCreate(savedInstanceState);
	}
	
	@Override
	public View onCreateView(LayoutInflater inflater, ViewGroup container,
			Bundle savedInstanceState) {
		// TODO Auto-generated method stub
		mRootView = inflater.inflate(R.layout.dt_viewpager_lay,container, false);
		return mRootView;
	}
	
	@Override
	public void onActivityCreated(Bundle savedInstanceState) {
		// TODO Auto-generated method stub
		super.onActivityCreated(savedInstanceState);
		find_ViewByid();
		initViews();
		initListener();
	}
	
	private void find_ViewByid(){
		tabGroup = (RadioGroup)mRootView.findViewById(R.id.viewpager_radiogroup);
		mViewPager = (ViewPager)mRootView.findViewById(R.id.view_pager_1);
		iv_nav_indicator = (ImageView)mRootView.findViewById(R.id.pager_nav_indicator);
	}
	
	private void initViews(){
		mInflater = LayoutInflater.from(getActivity());
		DisplayMetrics dm = new DisplayMetrics();
		getActivity().getWindowManager().getDefaultDisplay().getMetrics(dm);
		indicatorWidth = dm.widthPixels /3;
		
		LayoutParams cursor_Params = iv_nav_indicator.getLayoutParams();
		//Toast.makeText(getActivity(), "indicatorWidth is:"+indicatorWidth, 1).show();
		cursor_Params.width = indicatorWidth;
		iv_nav_indicator.setLayoutParams(cursor_Params);
		
		tabs = getActivity().getResources().getStringArray(R.array.dt_nav_tab_menu);
		for(int i=0;i<tabs.length;i++){
			RadioButton rb = (RadioButton) mInflater.inflate(R.layout.dt_nav_radiogroup_item, null);
			rb.setId(Constant.LOCAL_VIDEO+i);
			rb.setText(tabs[i]);
			rb.setLayoutParams(new LayoutParams(indicatorWidth,	LayoutParams.MATCH_PARENT));
			tabGroup.addView(rb);
			if(i==0){
				rb.setChecked(true);
			}
		}
		mAdapter = new TabFragmentPagerAdapter(getActivity().getSupportFragmentManager());
		mViewPager.setAdapter(mAdapter);
	}
	
	private void initListener(){
		mViewPager.setOnPageChangeListener(new OnPageChangeListener() {
			
			@Override
			public void onPageSelected(int position) {
				// TODO Auto-generated method stub
				if(tabGroup!=null && tabGroup.getChildCount()>position){
					((RadioButton)tabGroup.getChildAt(position)).performClick();
				}
			}
			
			@Override
			public void onPageScrolled(int arg0, float arg1, int arg2) {
				// TODO Auto-generated method stub
				
			}
			
			@Override
			public void onPageScrollStateChanged(int arg0) {
				// TODO Auto-generated method stub
				
			}
		});
		
		tabGroup.setOnCheckedChangeListener(new OnCheckedChangeListener() {
			@Override
			public void onCheckedChanged(RadioGroup group, int checkedId) {
				// TODO Auto-generated method stub
				RadioButton radioButton = (RadioButton)tabGroup.getChildAt(checkedId);
				if(radioButton!=null){
					
					TranslateAnimation animation = new TranslateAnimation(currentIndicatorLeft, 
							radioButton.getLeft(), 0f, 0f);
					animation.setInterpolator(new LinearInterpolator());
					animation.setDuration(100);
					animation.setFillAfter(true);
					//Toast.makeText(getActivity(), "currentIndicatorLeft is:"+currentIndicatorLeft+"--radioButton.getLeft() is:"+radioButton.getLeft(), 1).show();
					iv_nav_indicator.setAnimation(animation);
					mViewPager.setCurrentItem(checkedId);
					currentIndicatorLeft = radioButton.getLeft();
					modeListener.changeActionMode(checkedId);
					iv_nav_indicator.startAnimation(animation); //some where no need this sentence
					//getActivity().getActionBar().setSelectedNavigationItem(checkedId);
				}
			}
		});
	}
	
	
	
	
	@Override
	public void onDestroyView() {
		// TODO Auto-generated method stub
		super.onDestroyView();
	}
	
	@Override
	public void onPause() {
		// TODO Auto-generated method stub
		super.onPause();
	}
	
	Fragment videoft,audioft,fileft = null;
	public class TabFragmentPagerAdapter extends FragmentPagerAdapter{
		public TabFragmentPagerAdapter(FragmentManager fm) {
			super(fm);
		}

		@Override
		public Fragment getItem(int arg0) {
			switch (arg0) {
			case 0:
				if(videoft==null)
					videoft = new VideoUIFragment();
				Bundle args = new Bundle();
				args.putString(Constant.ARGUMENTS_NAME, tabs[arg0]);
				videoft.setArguments(args);
				return videoft;
			case 1:
				if(audioft==null)
					audioft = new AudioUIFragment();
				return audioft;
			case 2:
				if(fileft == null)
					fileft = new FilesUIFragment();
				return fileft;
			default:
				break;
			}
			return null;
		}

		@Override
		public int getCount() {
			return tabs.length;
		}
		
	}
	
	public interface ChangeActionModeListener{
		public void changeActionMode(int mode);
	}
}
