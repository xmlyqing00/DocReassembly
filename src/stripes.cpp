#include <stripes.h>

Stripes::Stripes() {
    stripes_n = 0;
    stripes.clear();
}

void Stripes::push(const cv::Mat & stripe_img) {
    stripes.push_back(stripe_img.clone());
    stripes_n = stripes.size();
}

bool Stripes::reassemble(Composition comp_mode) {
    
    switch (comp_mode) {
        case Stripes::GREEDY:
            return reassemble_greedy();
        default:
            return false;
    }

}

double Stripes::m_metric_word(const cv::Mat & frag0, const cv::Mat & frag1) {

}

void Stripes::merge_frags(const cv::Mat & in_frag0, const cv::Mat & in_frag1, cv::Mat & out_frag) {

    assert(in_frag0.rows == in_frag1.rows);

    cv::Size out_frag_size(in_frag0.cols + in_frag1.cols, in_frag0.rows);
    out_frag = cv::Mat(out_frag_size, CV_8UC3);
    in_frag0.copyTo(out_frag);
}

bool Stripes::reassemble_greedy() {

    double max_avg_m_score = 0;

    for (int i = 0; i < stripes_n; i++) {

        vector<bool> inclusion_flag(stripes_n, false);
        cv::Mat frag = stripes[i].clone();
        inclusion_flag[i] = true;
        double avg_m_score = 0;
        
        for (int inclusion_cnt = 1; inclusion_cnt < stripes_n; inclusion_cnt++) {
            
            double max_m_score = 0;
            int max_m_score_idx = 0;

            for (int j = 0; j < stripes_n; j++) {

                if (inclusion_flag[j]) continue;

                double m_score = m_metric_word(frag, stripes[i]);
                if (max_m_score < m_score) {
                    max_m_score = m_score;
                    max_m_score_idx = j;
                }

            }

            inclusion_flag[max_m_score_idx] = true;
            avg_m_score += max_m_score;
            merge_frags(frag, stripes[max_m_score_idx], frag);

        }

        avg_m_score /= stripes_n - 1;
        if (max_avg_m_score < avg_m_score) {
            max_avg_m_score = avg_m_score;
        }

    }

    return true;

}