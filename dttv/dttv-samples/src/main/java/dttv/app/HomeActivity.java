package dttv.app;


import android.support.design.widget.FloatingActionButton;
import android.support.design.widget.TabLayout;
import android.support.v4.app.Fragment;
import android.support.v4.view.ViewPager;

import java.util.ArrayList;
import java.util.List;

import butterknife.BindView;
import dttv.app.adapter.HomeAdapter;
import dttv.app.base.SimpleActivity;
import dttv.app.frament.BiliVideoFragment;
import dttv.app.frament.LocalVideoFragment;
import dttv.app.utils.ActivityUtils;
import dttv.app.widget.FloatingActionMenu;


public class HomeActivity extends SimpleActivity {

    @BindView(R.id.tab_home_main)
    TabLayout mTabLayout;
    @BindView(R.id.vp_home_main)
    ViewPager mViewPager;

    @BindView(R.id.fab_menu_circle)
    FloatingActionMenu menuCircle;

    String[] tabTitle = new String[]{"本地视频","网络视频","其他","热门"};
    List<Fragment> fragments = new ArrayList<Fragment>();

    HomeAdapter mAdapter;

    /*@Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_home);
        initEventAndData();
    }*/



    public void initEventAndData() {
        menuCircle.setIsCircle(true);
        menuCircle.setmItemGap(30);

        fragments.add(new LocalVideoFragment());
        fragments.add(new BiliVideoFragment());
        fragments.add(new LocalVideoFragment());
        fragments.add(new LocalVideoFragment());
        mAdapter = new HomeAdapter(getSupportFragmentManager(),fragments);
        //mViewPager = (ViewPager)findViewById(R.id.vp_home_main);
        //mTabLayout = (TabLayout)findViewById(R.id.tab_home_main);
        mViewPager.setAdapter(mAdapter);

        //TabLayout配合ViewPager有时会出现不显示Tab文字的Bug,需要按如下顺序
        mTabLayout.addTab(mTabLayout.newTab().setText(tabTitle[0]));
        mTabLayout.addTab(mTabLayout.newTab().setText(tabTitle[1]));
        mTabLayout.addTab(mTabLayout.newTab().setText(tabTitle[2]));
        mTabLayout.addTab(mTabLayout.newTab().setText(tabTitle[3]));
        mTabLayout.setupWithViewPager(mViewPager);
        mTabLayout.getTabAt(0).setText(tabTitle[0]);
        mTabLayout.getTabAt(1).setText(tabTitle[1]);
        mTabLayout.getTabAt(2).setText(tabTitle[2]);
        mTabLayout.getTabAt(3).setText(tabTitle[3]);

        menuCircle.setOnMenuItemClickListener(new FloatingActionMenu.OnMenuItemClickListener() {
            @Override
            public void onMenuItemClick(FloatingActionMenu fam, int index, FloatingActionButton item) {
                switch (index){
                    case 0:
                        ActivityUtils.startIntent(mContext,FileBrowserActivity.class);
                        break;
                    case 1:

                        break;
                    case 2:
                        ActivityUtils.startIntent(mContext,SettingActivity.class);
                        break;
                }
            }
        });
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
    }

    @Override
    protected int getLayout() {
        return R.layout.activity_home;
    }
}
