import Vue from "vue";
import Navigation from "./components/Header.vue";
import StoreInit from "./components/StoreInit.vue";
import router from "./router";
import { store } from "./store/store";
import axios from "axios";
import Paginate from 'vuejs-paginate'

Vue.component('paginate', Paginate)

window.axios = axios;

Vue.config.productionTip = false;

new Vue({
    el: "#app",
    store: store,
    router,
    components: { Navigation, StoreInit }
});
