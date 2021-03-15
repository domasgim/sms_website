<template>
  <div>
    <div class="container-messages" v-show="messagesCount > 0">
      <button class="filter-toggler"  @click="showFilters = !showFilters">
        <h2>Filter</h2>
        <img src="/images/triangle.svg" :class="{ 'rotate' : showFilters }">
      </button>
      <div class="filters" v-if="showFilters">
        <TextInput type="text" v-model="filterNumber" :label="numberTitle" :maxLength="15"/>
        <TextInput type="text" v-model="filterContent" label="Message"/>
        <TextInput type="text" v-model="filterDate" :label="dateTitle"/>
      </div>
      <Message v-for="message in messages" :key="message.id" :item="message" :numberTitle="numberTitle" :dateTitle="dateTitle"/>
      <h1 v-if="messages.length === 0" class="no-message">No messages found!</h1>
      <div class="to-top" @click="toTop" v-if="showToTop"><img src="/images/triangle-empty.svg"></div>
      <Paginate v-show="type === 'received' && messages.length > 0"
      :value="currentPage" 
      :pageCount="pageCountReceived"
      :margin-pages="2"
      :prevText="'←'"
      :nextText="'→'"
      :containerClass="'pagination'"
      :pageClass="'page-item'"
      :click-handler="clickCallback"
      />
      <Paginate v-show="type === 'sent' && messages.length > 0"
      :value="currentPage" 
      :pageCount="pageCountSent"
      :margin-pages="2"
      :prevText="'←'"
      :nextText="'→'"
      :containerClass="'pagination'"
      :pageClass="'page-item'"
      :click-handler="clickCallback"
      />
    </div>
    <h1 v-show="messagesCount === 0" class="no-message">No messages!</h1>
  </div>
</template>

<script>
import Message from '../components/Message.vue'
import TextInput from "../components/TextInput.vue";
import Paginate from 'vuejs-paginate'
export default {
  name: "Messages",
  components: { Message, TextInput, Paginate },
  props: {
    type: {
      type: String,
      default: 'received'
    }
  },
  data() {
    return {
      showToTop: false,
      showFilters: false,
      filterNumber: '',
      filterContent: '',
      filterDate: '',
      messagesCount: 0,
      numberTitle: 'Sender',
      dateTitle: 'Time received'
    };
  },
  computed: {
    messages () {
      let messages = []
      if (this.type === 'received') {
        this.numberTitle = 'Sender'
        this.dateTitle = 'Time received'
        this.messagesCount = this.$store.state.receivedMessages.length
        messages = this.$store.getters.getReceivedMessages
      } else {
        this.numberTitle = 'Recipient'
        this.dateTitle = 'Time sent'
        this.messagesCount = this.$store.state.sentMessages.length
        messages = this.$store.getters.getSentMessages
      }
      return messages
    },
    pageCountReceived () {
      return this.$store.state.pageCountReceived
    },
    pageCountSent () {
      return this.$store.state.pageCountSent
    },
    currentPage () {
      return this.$store.state.currentPage
    }
  },
  methods: {
    toTop () {
      document.body.scrollTop = 0; // For Safari
      document.documentElement.scrollTop = 0; // For Chrome, Firefox, IE and Opera
    },
    handleScroll (event) {
      if (document.body.scrollTop > 100 || document.documentElement.scrollTop > 100) {
        this.showToTop = true
      } else {
        this.showToTop = false
      }
    },
    clickCallback (pageNum) {
      this.$store.commit('updateCurrentPage', pageNum)
    },
  },
  created () {
    window.addEventListener('scroll', this.handleScroll);
  },
  destroyed () {
    window.removeEventListener('scroll', this.handleScroll);
  },
  watch: {
    numberTitle () {
      this.filterNumber = ''
      this.filterContent = ''
      this.filterDate = ''
    },
    $route (to, from) {
      this.$store.commit('updateCurrentPage', 1)
    },
    filterNumber () {
      this.$store.commit('setFilterNumber', this.filterNumber)
    },
    filterContent () {
      this.$store.commit('setFilterContent', this.filterContent)
    },
    filterDate () {
      this.$store.commit('setFilterDate', this.filterDate)
    }
  }
};
</script>

<style lang="scss" scoped>
.container-messages {
  position: relative;
  margin: 0 auto;
  width:90%;
  display: flex;
  flex-direction: column;
}
.no-message {
  text-align: center;
}
.to-top {
  width: 40px;
  height: 40px;
  font-size: 1.5em;
  font-weight: 900;
  text-align: center;
  position: absolute;
  bottom: -30px;
  right: -60px;
  background-color: #0054a6;
  color: #fff;
  border-radius: 50%;
  text-align: center;
  z-index: 5;
  cursor: pointer;
  user-select: none;
  opacity: 0.5;
  img {
    height: 30px;
    padding-top: 10px;
    filter: invert(93%) sepia(2%) saturate(159%) hue-rotate(170deg) brightness(100%) contrast(109%);
  }
  &:hover {
    opacity: 1;
  }
}
.filter-toggler {
  display: flex;
  flex-direction: row;
  line-height: 32px;
  align-items: center;
  justify-content: center;
  margin-bottom: 1.5em;
  cursor: pointer;
  background-color: #fff;
  border: 1.5px solid #0054a6;
  border-radius: 4px;
  text-transform: uppercase;
  color: #0054a6;
  &:hover {
    background-color: rgba(0, 0, 0, 0.025);
  }
  h2 {
    margin-right: 6px;
  }
  img {
    width: 12px;
    height: 12px;
    transform: rotate(180deg);
    filter: invert(21%) sepia(84%) saturate(1809%) hue-rotate(194deg) brightness(94%) contrast(102%);
  }
  .rotate {
    transform: rotate(360deg);
  }
}
.filters {
  margin-bottom: 1em;
  display: flex;
  flex-direction: row;
  justify-content: space-between;
  .flex-column {
    margin-right: 0;
    width: 30%;
  }
}
@media only screen and (max-width: 780px) {
  .to-top {
    right: -40px;
  }
  .filters {
    flex-direction: column;
    .flex-column {
      width: 100%;
    }
  }
}
</style>
